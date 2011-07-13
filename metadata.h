#ifndef MULTISNAP_METADATA_H
#define MULTISNAP_METADATA_H

#include "block.h"
#include "transaction_manager.h"
#include "btree.h"

#include <string>

#include <boost/shared_ptr.hpp>

//----------------------------------------------------------------

// FIXME: make a const
#define BLOCK_SIZE 4096

namespace multisnap {
	typedef uint64_t sector_t;

	struct device_details_disk {
		__le64 dev_size;
		__le64 mapped_blocks;
		__le64 transaction_id;  /* when created */
		__le32 creation_time;
		__le32 snapshotted_time;
	} __attribute__ ((packed));

	struct device_details {
		uint64_t dev_size;
		uint64_t mapped_blocks;
		uint64_t transaction_id;  /* when created */
		uint32_t creation_time;
		uint32_t snapshotted_time;
	};

	struct detail_traits {
		typedef device_details_disk disk_type;
		typedef device_details value_type;

		static value_type construct(void *data) {
			struct device_details_disk disk;
			struct device_details cpu;

			::memcpy(&disk, data, sizeof(disk));
			cpu.dev_size = to_cpu<uint64_t>(disk.dev_size);
			cpu.mapped_blocks = to_cpu<uint64_t>(disk.mapped_blocks);
			cpu.transaction_id = to_cpu<uint64_t>(disk.transaction_id);
			cpu.creation_time = to_cpu<uint32_t>(disk.creation_time);
			cpu.snapshotted_time = to_cpu<uint32_t>(disk.snapshotted_time);

			return cpu;
		}
	};
#if 0
	class dev_traits {
	public:
		typedef base::__le64 disk_type;
		typedef persistent_data::btree<1, uint64_traits, BLOCK_SIZE> value_type;

		static value_type construct(void *data) {
			uint64_t root = uint64_traits::construct(data);

			return value_type
		}
	};
#endif

	class metadata {
	public:
		typedef boost::shared_ptr<metadata> ptr;
		typedef persistent_data::block_address block_address;

		metadata(std::string const &metadata_dev,
			 sector_t data_block_size,
			 persistent_data::block_address nr_data_blocks);
		~metadata();

		void commit();

		typedef uint32_t dev_t;
		void create_thin(dev_t dev);
		void create_snap(dev_t dev, dev_t origin);
		void del(dev_t);

		void set_transaction_id(uint64_t id);
		uint64_t get_transaction_id() const;

		block_address get_held_root() const;

		block_address alloc_data_block();
		void free_data_block(block_address b);

		// accessors
		block_address get_nr_free_data_blocks() const;
		sector_t get_data_block_size() const;
		block_address get_data_dev_size() const;

		class thin {
		public:
			typedef boost::shared_ptr<thin> ptr;

			dev_t get_dev_t() const;

			typedef boost::optional<block_address> maybe_address;
			maybe_address lookup(block_address thin_block);
			void insert(block_address thin_block, block_address data_block);
			void remove(block_address thin_block);

			void set_snapshot_time(uint32_t time);

			persistent_data::block_address get_mapped_blocks() const;
			void set_mapped_blocks(persistent_data::block_address count);

		private:
			dev_t dev_;
			metadata::ptr metadata_;
		};

		thin::ptr open(dev_t);

	private:
		friend class thin;

		bool device_exists(dev_t dev) const;

		uint32_t time_;

		persistent_data::transaction_manager<BLOCK_SIZE>::ptr tm_;

		typedef persistent_data::btree<1, detail_traits, BLOCK_SIZE> detail_tree;
		typedef persistent_data::btree<1, uint64_traits, BLOCK_SIZE> dev_tree;
		typedef persistent_data::btree<2, uint64_traits, BLOCK_SIZE> mapping_tree;
		typedef persistent_data::btree<1, uint64_traits, BLOCK_SIZE> single_mapping_tree;

		detail_tree details_;
		dev_tree mappings_top_level_;
		mapping_tree mappings_;
	};
};

//----------------------------------------------------------------

#endif